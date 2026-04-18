# Migration Lane

This folder stores migration planning and audit artifacts for controlled import from legacy NovaForge sources.

## Rule
Legacy source repositories and zip archives are migration inputs. They are not permanent runtime repo contents.

## Use
- place audit reports in `Intake/reports/`
- place file-by-file manifests in `Intake/manifests/`
- place approved staged imports in `Intake/staging/`
- only promote staged files into runtime folders after validation
